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
#include "sgx3DsrvLink_priv.h"
#include <linux/src/osa/include/osa_mem.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#ifdef USE_STANDALONE_LUT
#include "standalone/GAlignLUT.c"
#include "standalone/BlendLUT3D.c"
#endif

extern srv_coords_t srv_coords[];

extern float car_x;
extern float car_y;
extern float car_z;
extern float bowl_x;
extern float bowl_y;
extern float bowl_z;
extern float car_anglex;
extern float car_angley;
extern float car_anglez;
extern float bowl_anglex;
extern float bowl_angley;
extern float bowl_anglez;
extern float car_scale;
extern float bowl_scale;
#if 0
static int carnbowl;
static int translatenrotate;
static float delta = 1.0f;
extern GLenum render_mode;
#endif

static int num_coordsets;
static int num_iterations = 60;
//static int current_iteration;
//static int current_coordset;

pthread_t scan_thread;

static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    printf("Terminal setting flags:0x%x", oldt.c_lflag);
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}

#define COORD_TRANSITION(x) \
  x = srv_coords[i].x + j * ((srv_coords[(i+1)%num_coordsets].x - srv_coords[i].x)/num_iterations)

#if 1
void scan_thread_function()
{
//	char input;
	int i, j;
	disable_waiting_for_enter();
	num_coordsets = 3;// (int)(sizeof(srv_coords)/sizeof(srv_coords_t));
	//printf("Number of coordinate sets: %d", num_coords);
	while(1)
	{
		for (i = 0; i < num_coordsets; i++)
		{
			for(j = 0; j < num_iterations; j++)
			{
				COORD_TRANSITION(car_x);
				COORD_TRANSITION(car_y);
				COORD_TRANSITION(car_z);
				COORD_TRANSITION(car_anglex);
				COORD_TRANSITION(car_angley);
				COORD_TRANSITION(car_anglez);
				COORD_TRANSITION(car_scale);
				COORD_TRANSITION(bowl_x);
				COORD_TRANSITION(bowl_y);
				COORD_TRANSITION(bowl_z);
				COORD_TRANSITION(bowl_anglex);
				COORD_TRANSITION(bowl_angley);
				COORD_TRANSITION(bowl_anglez);
				COORD_TRANSITION(bowl_scale);
				//car_x = srv_coords[i].car_x + j * ((srv_coords[(i+1)%num_coordsets].car_x - srv_coords[i].car_x)/num_iterations);
				usleep(66000);
			}
			sleep(10);
			if (i >= num_coordsets)
				i = 0;
		}
	}
}
#else
void scan_thread_function()
{
	char input;
	disable_waiting_for_enter();
	while(1)
	{
		//scanf("%c",&input);
		input = getchar();
		switch(input)
		{
		case 'c':
			carnbowl = 1;
			break;
		case 'b':
			carnbowl = 0;
			break;
		case 't':
			translatenrotate = 1;
			break;
		case 'r':
			translatenrotate = 0;
			break;
		case 'i':
			if(delta < 0)
				delta = -delta;
			break;
		case 'd':
			if(delta > 0)
				delta = -delta;
			break;
		case 'x':
			if(translatenrotate == 1)
			{//Translate
				if(carnbowl == 1)
				{
					car_x += delta;
				}
				else
				{
					bowl_x += delta;
				}
			}
			else
			{//Rotate
				if(carnbowl == 1)
				{
					car_anglex += delta;
				}
				else
				{
					bowl_anglex += delta;
				}
			}
			break;
		case 'y':
			if(translatenrotate == 1)
			{//Translate
				if(carnbowl == 1)
				{
					car_y += delta;
				}
				else
				{
					bowl_y += delta;
				}
			}
			else
			{//Rotate
				if(carnbowl == 1)
				{
					car_angley += delta;
				}
				else
				{
					bowl_angley += delta;
				}
			}
			break;
		case 'z':
			if(translatenrotate == 1)
			{//Translate
				if(carnbowl == 1)
				{
					car_z += delta;
				}
				else
				{
					bowl_z += delta;
				}
			}
			else
			{//Rotate
				if(carnbowl == 1)
				{
					car_anglez += delta;
				}
				else
				{
					bowl_anglez += delta;
				}
			}
			break;
		case 's':
			if(carnbowl == 1)
			{
				car_scale += 0.1*delta;
			}
			else
			{
				bowl_scale += 0.1*delta;
			}
			break;
		case 'm':
			if(delta >=0)
				delta += 0.1f;
			else
				delta -= 0.1f;
			printf("Delta: %f\n", delta);
			break;
		case 'l' :
			if(delta >=0)
				delta -= 0.1f;
			else
				delta += 0.1f;
			printf("Delta: %f\n", delta);
			break;
		case 'q':
			car_x = 0;
			car_y = 0;
			car_z = -2.5;
			bowl_x = 0;
			bowl_y = 0;
			bowl_z = -4.5;
			car_anglex = 0;
			car_angley = 0;
			car_anglez = 0;
			bowl_anglex = 0;
			bowl_angley = 0;
			bowl_anglez = 0;
			car_scale = 0;
			bowl_scale = 0;
			break;
		case 'n':
			if(render_mode == GL_LINE_STRIP)
				render_mode = GL_TRIANGLE_STRIP;
			else
				render_mode = GL_LINE_STRIP;
			break;
		default:
			break;
		}
		printf("Car: %f, %f, %f <% f, %f, %f> s=%f/n Bowl: %f, %f, %f <%f, %f, %f> s=%f\n",
				car_x,
				car_y,
				car_z,
				car_anglex,
				car_angley,
				car_anglez,
				car_scale,
				bowl_x,
				bowl_y,
				bowl_z,
				bowl_anglex,
				bowl_angley,
				bowl_anglez,
				bowl_scale);
	}
}
#endif

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
Int32 Sgx3DsrvLink_drvCreate(Sgx3DsrvLink_Obj *pObj,
                             Sgx3DsrvLink_CreateParams *pPrm)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    System_DrmOpenPrms      drmOpenPrm;
    UInt32                  inQue, channelId;
    Sgx3DsrvLink_InputQueId inputQId;
    UInt32                  prevChInfoFlags;
    System_LinkChInfo     * pPrevChInfo;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Create in progress !!!\n");
#endif

    memcpy(&pObj->createArgs, pPrm, sizeof(*pPrm));

    printf("Creating scan thread\n");
    pthread_create(&scan_thread, NULL, (void *)&scan_thread_function, NULL);
//    pthread_create(&scan_thread, NULL, (void *)&scan_thread_function, NULL);

    OSA_assert(pPrm->numInQue <= SGX3DSRV_LINK_IPQID_MAXIPQ);

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

    OSA_assert(pObj->createArgs.inBufType[SGX3DSRV_LINK_IPQID_MULTIVIEW] ==
                     SYSTEM_BUFFER_TYPE_VIDEO_FRAME_CONTAINER);
    OSA_assert(pObj->createArgs.inBufType[SGX3DSRV_LINK_IPQID_PALUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);
    OSA_assert(pObj->createArgs.inBufType[SGX3DSRV_LINK_IPQID_GALUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);
    OSA_assert(pObj->createArgs.inBufType[SGX3DSRV_LINK_IPQID_BLENDLUT] ==
                     SYSTEM_BUFFER_TYPE_METADATA);

    inputQId = SGX3DSRV_LINK_IPQID_MULTIVIEW;
    channelId = 0;
    pPrevChInfo   =
        &(pObj->inQueInfo[inputQId].chInfo[channelId]);

    OSA_assert(pObj->createArgs.maxOutputWidth  <= SGX3DSRV_LINK_OUTPUT_FRAME_WIDTH);
    OSA_assert(pObj->createArgs.maxOutputHeight <= SGX3DSRV_LINK_OUTPUT_FRAME_HEIGHT);

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

    /*
     * Creation of local input Qs for SGX3DSRV_LINK_IPQID_MULTIVIEW and
     * SGX3DSRV_LINK_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = SGX3DSRV_LINK_IPQID_MULTIVIEW;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SGX3DSRV_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_PALUT;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SGX3DSRV_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_GRPX;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SGX3DSRV_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_VIDMOSAIC;
    status  = OSA_queCreate(&(pObj->localInputQ[inputQId].queHandle),
                               SGX3DSRV_LINK_MAX_LOCALQUEUELENGTH);
    OSA_assert(status == SYSTEM_LINK_STATUS_SOK);

    drmOpenPrm.connector_id = -1;
    drmOpenPrm.displayWidth = 1920;
    drmOpenPrm.displayHeight = 1080;

    status = System_drmOpen(&pObj->drmObj, &drmOpenPrm);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    status = System_eglOpen(&pObj->eglObj, &pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    status = System_drmSetMode(&pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    pObj->render3DSRVObj.screen_width = pObj->eglObj.width;
    pObj->render3DSRVObj.screen_height = pObj->eglObj.height;
    status = SgxRender3DSRV_setup(&pObj->render3DSRVObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

    OSA_resetLatency(&pObj->linkLatency);
    OSA_resetLatency(&pObj->srcToLinkLatency);

    pObj->numInputChannels = 1;
    OSA_resetLinkStatistics(&pObj->linkStats, pObj->numInputChannels, 1);

    pObj->isFirstFrameRecv     = FALSE;
    pObj->receivedGALUTFlag    = FALSE;
    pObj->receivedBlendLUTFlag = FALSE;
    pObj->receivedFirstPALUTFlag = FALSE;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Create Done !!!\n");
#endif

    return SYSTEM_LINK_STATUS_SOK;
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
Int32 Sgx3DsrvLink_drvPrintStatistics(Sgx3DsrvLink_Obj *pObj)
{
    OSA_printLinkStatistics(&pObj->linkStats, "SGX3DSRV", TRUE);

    OSA_printLatency("SGX3DSRV",
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
Int32 Sgx3DsrvLink_getInputFrameData(Sgx3DsrvLink_Obj * pObj)
{
    Sgx3DsrvLink_InputQueId      inputQId;
    Int32                        status    = SYSTEM_LINK_STATUS_SOK;
    UInt32                       bufId;
    System_BufferList            inputBufList;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSysBufferInput;
    Sgx3DsrvLink_CreateParams  * pPrm;

    pPrm = &pObj->createArgs;

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_MULTIVIEW and queue them up locally.
     */
    inputQId = SGX3DSRV_LINK_IPQID_MULTIVIEW;

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
    inputQId = SGX3DSRV_LINK_IPQID_PALUT;

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
    inputQId = SGX3DSRV_LINK_IPQID_GALUT;
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
            Vps_printf (" SGX3DSRV Link - GA LUT received !!! \n");
#endif
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_BLENDLUT and store latest copy locally.
     */
    inputQId = SGX3DSRV_LINK_IPQID_BLENDLUT;
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
            Vps_printf (" SGX3DSRV Link - Blending LUT received !!! \n");
#endif
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_GRPX and queue them up locally.
     */
    inputQId = SGX3DSRV_LINK_IPQID_GRPX;

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
#ifdef SYSTEM_DEBUG_DISPLAY
            Vps_printf (" SGX3DSRV Link - GRPX frame received !!! \n");
#endif
        }
    }

    /*
     * Get Input buffers from previous link for
     * Qid = SGX3DSRV_LINK_IPQID_VIDMOSAIC and queue them up locally.
     */
    inputQId = SGX3DSRV_LINK_IPQID_VIDMOSAIC;
    inputBufList.numBuf = 0;
    if (inputQId < pPrm->numInQue)
    {
        System_getLinksFullBuffers(
            pPrm->inQueParams[inputQId].prevLinkId,
            pPrm->inQueParams[inputQId].prevLinkQueId,
            &inputBufList);
    }
    
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
        }
    }

    pObj->linkStats.newDataCmdCount++;

    return status;
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
volatile static unsigned int gDebug_IsCalibDone = 0;
Int32 Sgx3DsrvLink_drvDoProcessFrames(Sgx3DsrvLink_Obj *pObj)
{
    Sgx3DsrvLink_CreateParams  * pPrm;
    Sgx3DsrvLink_InputQueId      inputQId;
    UInt32                       channelId = 0;
    Int32                        status = SYSTEM_LINK_STATUS_SOK;
    System_BufferList            inputBufListReturn;
    System_Buffer              * pSystemBufferMultiview;
    System_Buffer              * pSystemBufferPALUT;
    System_Buffer              * pSystemGrpxBuffer;
    System_Buffer              * pSystemFcVidBuffer;
    System_VideoFrameBuffer    * pVideoBuffer;
    Bool                         isProcessCallDoneFlag;
    System_MetaDataBuffer      * pPALUTBuffer;
    System_MetaDataBuffer      * pGALUTBuffer;
    System_MetaDataBuffer      * pBlendLUTBuffer;
    System_VideoFrameCompositeBuffer *pVideoCompositeFrame;
    GLuint                       texYuv[4] = {0};
    System_EglTexProperty        texProp;
    System_DrmFBProperty         grpxfbProp;
    System_DrmFBProperty         fcfbProp;
    static int                   grpxfb = -1;
    static int                   fcfb = -1;
    int                          ret;

    pPrm = &pObj->createArgs;

    Sgx3DsrvLink_getInputFrameData(pObj);

    /*
     * Continous loop to perform synthesis as long as
     * input buffers are available.
     */
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
              &(pObj->localInputQ[SGX3DSRV_LINK_IPQID_MULTIVIEW].queHandle))>0
       )
       {

        pSystemBufferPALUT = NULL;
        status = OSA_queGet(
                    &(pObj->localInputQ[SGX3DSRV_LINK_IPQID_PALUT].
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

        /* Submit the FC Analytics buffer to DRM display */
        pSystemFcVidBuffer = NULL;
        status = OSA_queGet(
                  &(pObj->localInputQ[SGX3DSRV_LINK_IPQID_VIDMOSAIC].
                      queHandle),
                  (Int32 *) &pSystemFcVidBuffer,
                  OSA_TIMEOUT_NONE);

        if (pSystemFcVidBuffer != NULL && status == SYSTEM_LINK_STATUS_SOK)
        {
#ifdef SYSTEM_DEBUG_DISPLAY_RT
           Vps_printf(" SGX3DSRV Link: FC FB create start \n");
#endif
           inputQId = SGX3DSRV_LINK_IPQID_VIDMOSAIC;
           channelId = pSystemFcVidBuffer->chNum;

           pVideoBuffer = pSystemFcVidBuffer->payload;
           OSA_assert (pVideoBuffer!=NULL);

           fcfbProp.width      = pObj->inQueInfo[inputQId].chInfo[channelId].width;
           fcfbProp.height     = pObj->inQueInfo[inputQId].chInfo[channelId].height;
           fcfbProp.pitch[0]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[0];
           fcfbProp.pitch[1]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[1];
           fcfbProp.dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pObj->inQueInfo[inputQId].chInfo[0].flags);
           fcfb = System_drmGetFB(&pObj->drmObj, &fcfbProp, pVideoBuffer->bufAddr[0]);
#ifdef SYSTEM_DEBUG_DISPLAY_RT
           Vps_printf(" SGX3DSRV Link: FC FB created as %d\n", fcfb);
#endif
        }

        if(fcfb != -1)
        {
           ret = drmModeSetPlane(pObj->drmObj.drmFd, pObj->drmObj.fc_plane_id,
                                 pObj->drmObj.crtc_id[pObj->drmObj.curDisplay],
                                 fcfb, 0,
                                 (1920-25-640),
                                 (100+48),
                                 640,
                                 (360*2+50),
                                 0<<16,
                                 0<<16,
                                 640<<16,
                                 (360*2+50)<<16
                                 );
           if(ret)
              Vps_printf(" SGX3DSRV Link: DRM: drmModeSetPlane Failed for plane (%s) !!!\n", strerror(errno));
        }

        /* Submit the GRPX buffer to DRM display */
        pSystemGrpxBuffer = NULL;
        status = OSA_queGet(
                  &(pObj->localInputQ[SGX3DSRV_LINK_IPQID_GRPX].
                      queHandle),
                  (Int32 *) &pSystemGrpxBuffer,
                  OSA_TIMEOUT_NONE);

        if (pSystemGrpxBuffer != NULL && status == SYSTEM_LINK_STATUS_SOK)
        {
#ifdef SYSTEM_DEBUG_DISPLAY_RT
           Vps_printf(" SGX3DSRV Link: GRPX FB create start \n");
#endif
           inputQId = SGX3DSRV_LINK_IPQID_GRPX;
           channelId = pSystemGrpxBuffer->chNum;

           pVideoBuffer = pSystemGrpxBuffer->payload;
           OSA_assert (pVideoBuffer!=NULL);

           grpxfbProp.width      = pObj->inQueInfo[inputQId].chInfo[channelId].width;
           grpxfbProp.height     = pObj->inQueInfo[inputQId].chInfo[channelId].height;
           grpxfbProp.pitch[0]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[0];
           grpxfbProp.pitch[1]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[1];
           grpxfbProp.dataFormat = SYSTEM_LINK_CH_INFO_GET_FLAG_DATA_FORMAT(pObj->inQueInfo[inputQId].chInfo[0].flags);
           grpxfb = System_drmGetFB(&pObj->drmObj, &grpxfbProp, pVideoBuffer->bufAddr[0]);
#ifdef SYSTEM_DEBUG_DISPLAY_RT
           Vps_printf(" SGX3DSRV Link: GRPX FB created as %d\n", grpxfb);
#endif
        }

        if(grpxfb != -1)
        {
           ret = drmModeSetPlane(pObj->drmObj.drmFd, pObj->drmObj.grpx_plane_id,
                                 pObj->drmObj.crtc_id[pObj->drmObj.curDisplay],
                                 grpxfb, 0,
                                 0,
                                 0,
                                 1920,
                                 1080,
                                 0<<16,
                                 0<<16,
                                 1920<<16,
                                 1080<<16
                                 );
           if(ret)
              Vps_printf(" SGX3DSRV Link: DRM: drmModeSetPlane Failed for plane (%s) !!!\n", strerror(errno));
        }

        /*
         * Reaching here means output buffers are available.
         * Hence getting inputs from local Queus
         */
        pSystemBufferMultiview = NULL;
        status = OSA_queGet(
                  &(pObj->localInputQ[SGX3DSRV_LINK_IPQID_MULTIVIEW].
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

            inputQId = SGX3DSRV_LINK_IPQID_MULTIVIEW;

            texProp.width      = pObj->inQueInfo[inputQId].chInfo[channelId].width;
            texProp.height     = pObj->inQueInfo[inputQId].chInfo[channelId].height;
            texProp.pitch[0]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[0];
            texProp.pitch[1]   = pObj->inQueInfo[inputQId].chInfo[channelId].pitch[1];
            texProp.dataFormat = pObj->inDataFormat;

#ifndef USE_STANDALONE_LUT
            // Do we need to specify the stitched frame resolution ?
            // Do we need to set any other parameters in render3DSRVObj ?
            pObj->render3DSRVObj.LUT3D = (void *) pGALUTBuffer->bufAddr[0];
            pObj->render3DSRVObj.blendLUT3D = (void *) pBlendLUTBuffer->bufAddr[0];
            pObj->render3DSRVObj.PALUT3D = (void *) pPALUTBuffer->bufAddr[0];
#else
            //Use standalone version of LUTs
            //FOR DEBUG ONLY
            pObj->render3DSRVObj.LUT3D = (void *) GAlignLUT;
            pObj->render3DSRVObj.blendLUT3D = (void *) BlendLUT3D;
#endif

#if 0  // Enable Dumping out of the tables
            {
            FILE *fp1;
            FILE *fp2;

            gDebug_IsCalibDone++;
            if (gDebug_IsCalibDone == 250)
            {
                fp1=fopen("./3DLUT.bin", "w+b");
                fp2=fopen("./3DBLENDLUT.bin", "w+b");

                fwrite(pObj->render3DSRVObj.LUT3D, (9*POINTS_WIDTH*POINTS_HEIGHT*sizeof(LUT_DATATYPE)), 1, fp1);
                fwrite(pObj->render3DSRVObj.blendLUT3D, (2*POINTS_WIDTH*POINTS_HEIGHT*sizeof(BLENDLUT_DATATYPE)), 1, fp2);
                
                fclose(fp1);
                fclose(fp2);
            }
            }
#endif

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
                Vps_printf(" SGX3DSRV Link: ERROR: Recevied invalid buffer type !!!\n");
                OSA_assert(0);
            }
#if 1
            SgxRender3DSRV_renderFrame(
                            &pObj->render3DSRVObj,
                            &pObj->eglObj,
                            texYuv
                            );
#else
            OSA_assert (texYuv != NULL);
#endif
            OSA_updateLatency(&pObj->linkLatency,
                              pSystemBufferMultiview->linkLocalTimestamp);
            OSA_updateLatency(&pObj->srcToLinkLatency,
                              pSystemBufferMultiview->srcTimestamp);
        }

        status = System_drmEglSwapBuffers(&pObj->drmObj, &pObj->eglObj);

        if(status!=SYSTEM_LINK_STATUS_SOK)
        {
            Vps_printf(" SGX3DSRV Link: System_drmEglSwapBuffers() failed !!!\n");
        }

        isProcessCallDoneFlag = TRUE;

        /*
         * Releasing (Free'ing) Input buffers, since algorithm does not need
         * it for any future usage.
         */
        if (pSystemBufferMultiview != NULL)
        {
          inputQId                      = SGX3DSRV_LINK_IPQID_MULTIVIEW;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSystemBufferMultiview;
          if(inputBufListReturn.numBuf)
          {
              System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                          pPrm->inQueParams[inputQId].prevLinkQueId,
                                          &inputBufListReturn);
          }
        }

        if (pSystemBufferPALUT != NULL)
        {
          inputQId                      = SGX3DSRV_LINK_IPQID_PALUT;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSystemBufferPALUT;
          if(inputBufListReturn.numBuf)
          {
              System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                          pPrm->inQueParams[inputQId].prevLinkQueId,
                                          &inputBufListReturn);
          }
        }
#if 0
        if (pSystemGrpxBuffer != NULL)
        {
          inputQId                      = SGX3DSRV_LINK_IPQID_GRPX;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSystemGrpxBuffer;
          if(inputBufListReturn.numBuf)
          {
              System_putLinksEmptyBuffers(pPrm->inQueParams[inputQId].prevLinkId,
                                          pPrm->inQueParams[inputQId].prevLinkQueId,
                                          &inputBufListReturn);
          }
        }
#endif

        if (pSystemFcVidBuffer != NULL)
        {
          inputQId                      = SGX3DSRV_LINK_IPQID_VIDMOSAIC;
          inputBufListReturn.numBuf     = 1;
          inputBufListReturn.buffers[0] = pSystemFcVidBuffer;
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
Int32 Sgx3DsrvLink_drvDelete(Sgx3DsrvLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;
    Sgx3DsrvLink_InputQueId inputQId;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Delete in progress !!!\n");
#endif

    /*
     * Deletion of local input Qs for SGX3DSRV_LINK_IPQID_MULTIVIEW and
     * SGX3DSRV_LINK_IPQID_PALUT.
     * For ALGLINK_SYNTHESIS_IPQID_GALUT, always just one entry is kept.
     */
    inputQId = SGX3DSRV_LINK_IPQID_MULTIVIEW;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_PALUT;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_GRPX;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    inputQId = SGX3DSRV_LINK_IPQID_VIDMOSAIC;
    status = OSA_queDelete(&(pObj->localInputQ[inputQId].queHandle));
    UTILS_assert(status == SYSTEM_LINK_STATUS_SOK);

    status = System_drmClose(&pObj->drmObj);
    OSA_assert(status==SYSTEM_LINK_STATUS_SOK);

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Delete Done !!!\n");
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
Int32 Sgx3DsrvLink_drvStart(Sgx3DsrvLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Start in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Start Done !!!\n");
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
Int32 Sgx3DsrvLink_drvStop(Sgx3DsrvLink_Obj *pObj)
{
    Int32 status = SYSTEM_LINK_STATUS_SOK;

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Stop in progress !!!\n");
#endif

#ifdef SYSTEM_DEBUG_DISPLAY
    Vps_printf(" SGX3DSRV Link: Stop Done !!!\n");
#endif

    return status;
}

/* Nothing beyond this point */

